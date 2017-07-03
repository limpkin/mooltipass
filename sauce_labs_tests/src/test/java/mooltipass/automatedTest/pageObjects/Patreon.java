package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Patreon extends AbstractPage{
	
	public Patreon (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//div[@data-reactid='.0.0.2.2.1']//a[contains(text(),'Log In')]")
	private WebElement loginBtn;
	@FindBy(xpath = "//input[@name='email']")
	private WebElement email;

	@FindBy(xpath = "//input[@name='password']")
	private WebElement password;
	
	@FindBy(xpath = "//div[contains(text(),'Log in')]")
	private WebElement submitLogin;
	
	
	@FindBy(xpath = "//a[contains(@href,'logout')]")
	private WebElement logoutBtn;
									
	@FindBy(xpath = "//div[contains(@style,'https://c8.patreon.com/2/100/6446993')]")
	private WebElement dashBoard;
	
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

		waitUntilAppears(submitLogin);
		submitLogin.click();
	}
	public void goTodDashboard()
	{
		waitUntilAppears(dashBoard);
		hover(dashBoard);	
	}
	public void logout(){
		logoutBtn.click();
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath( "//div[contains(text(),'mooltipass')]"));
		return isElementPresent(By.xpath( "//div[contains(text(),'mooltipass')]"));
	}
	public boolean checkAtLoginPage(){
		waitUntilAppears(By.xpath("//input[@name='email']"));
		return isElementPresent(By.xpath("//input[@name='email']"));
	}

}
