package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class PayPal  extends AbstractPage{
	
	public PayPal (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "ul-btn")
	private WebElement loginBtn;
	@FindBy(id = "email")
	private WebElement email;
	
	@FindBy(id = "password")
	private WebElement password;
	
	@FindBy(id = "btnLogin")
	private WebElement submitLogin;

	@FindBy(xpath = "//a[@href='/myaccount/logout']")
	private WebElement logoutBtn;
	
	public void goToLogin(){	
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	
	public void submit(){

		submitLogin.click();
	}

	public void logout(){		
		logoutBtn.click();
	}
	
	public boolean checkLogin(){	
		waitUntilAppears(By.xpath("//a[@href='/myaccount/logout']"));
		return isElementPresent(By.xpath("//a[@href='/myaccount/logout']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("email"));
	}


}
