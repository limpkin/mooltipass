package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class TurkishAirlines extends AbstractPage{

	public TurkishAirlines (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//span[contains(text(),'SIGN IN')]")
	private WebElement loginBtn;
	
	@FindBy(id = "username1")
	private WebElement email;

	@FindBy(id = "password")
	private WebElement password;
	
	@FindBy(id = "signinBTN")
	private WebElement submitLogin;
	
	@FindBy(id = "signoutBTN")
	private WebElement logoutBtn;
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);
		loginBtn.click();
	}
	
	public void submit(){
		submitLogin.click();
	}
	
	public void logout(){
		waitUntilAppears(logoutBtn);
		clickWithJS(logoutBtn);
	}
	
	public boolean checkLogin(){		        
		waitUntilAppears(By.id( "signoutBTN"));
		return isElementPresent(By.id( "signoutBTN"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("username1"));
	}
	
}
