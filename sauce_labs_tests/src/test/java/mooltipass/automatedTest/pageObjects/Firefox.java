package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Firefox extends AbstractPage{

	public Firefox(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	@FindBy(xpath = "//input[@type='email']")
	private WebElement email;

	@FindBy(xpath = "//input[@type='password']")
	private WebElement password;

	@FindBy(xpath = "//button[@id='submit-btn']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[contains(text(),'Log in')]")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//a[text()='Log out']")
	private WebElement logoutBtn;
	

	@FindBy(xpath = "//a[@title='citesting@themooltipass.com']")
	private WebElement user;


	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}
	
	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){

		waitUntilAppears(loginBtn);	
		clickWithAction(loginBtn);
	}

	public void submit(){
		waitUntilAppears(submitLogin);
		submitLogin.click();
	}
	
	public void clickuser()
	{
		hover(user);
		
	}
	public void logout(){
		waitUntilAppears(logoutBtn);
		clickWithAction(logoutBtn);
	}
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//a[@title='citesting@themooltipass.com']"));
		return isElementPresent(By.xpath("//a[@title='citesting@themooltipass.com']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("submit-btn"));
	}
}

